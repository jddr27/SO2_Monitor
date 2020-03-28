package main

import (
	"encoding/json"
	"fmt"
	"html/template"
	"io"
	"io/ioutil"
	"net/http"
	"time"

	"github.com/labstack/echo"
	"github.com/labstack/echo/middleware"

	pusher "github.com/pusher/pusher-http-go"
	"github.com/shirou/gopsutil/process"
)

// We register the Pusher client
var client = pusher.Client{
	AppID:   "869082",
	Key:     "9afe9110eb9f886eed9e",
	Secret:  "ded06f23b55001902cd4",
	Cluster: "us2",
	Secure:  true,
}

// cpuData : struct del CPU
type cpuData struct {
	Total  float64
	Idle   float64
	Per    float64
	Tiempo string
}

// ramData : struct de la RAM
type ramData struct {
	Total  float64
	Usado  float64
	Per    float64
	Tiempo string
}

// Hijo : son los procesos hijos
type Hijo struct {
	Pid    int
	Nombre string
}

// Padre : son los procesos padres
type Padre struct {
	Pid    int
	Nombre string
	Estado string
	Hijos  []Hijo
}

type procsData struct {
	Proce []Padre
	Todos int
	Corr  int
	Durm  int
	Para  int
	Zomb  int
}

func setInterval(ourFunc func(), milliseconds int, async bool) chan bool {
	// How often to fire the passed in function in milliseconds
	interval := time.Duration(milliseconds) * time.Millisecond

	// Setup the ticker and the channel to signal
	// the ending of the interval
	ticker := time.NewTicker(interval)
	clear := make(chan bool)

	// Put the selection in a go routine so that the for loop is none blocking
	go func() {
		for {
			select {
			case <-ticker.C:
				if async {
					// This won't block
					go ourFunc()
				} else {
					// This will block
					ourFunc()
				}
			case <-clear:
				ticker.Stop()
				return
			}
		}
	}()
	// We return the channel so we can pass in
	// a value to it to clear the interval
	return clear
}

func obtenerRAM(c echo.Context) error {
	setInterval(func() {
		currentTime := time.Now()

		data, err := ioutil.ReadFile("/proc/201503393_ram")
		if err != nil {
			fmt.Println("File reading error", err)
			return //c.String(http.StatusConflict, "File reading error")
		}
		fmt.Println("Contents of file:", string(data))

		ramJSON := string(data)
		var newRAMData ramData
		json.Unmarshal([]byte(ramJSON), &newRAMData)
		newRAMData.Per = ((newRAMData.Usado * 100.0) / newRAMData.Total)
		newRAMData.Tiempo = currentTime.Format("2006.01.02 15:04:05")

		client.Trigger("ramPercentage", "addNumber", newRAMData)
	}, 2500, true)

	return c.String(http.StatusOK, "RAM begun")
}

func obtenerCPU(c echo.Context) error {
	setInterval(func() {
		currentTime := time.Now()

		data, err := ioutil.ReadFile("/proc/201503393_cpu")
		if err != nil {
			fmt.Println("File reading error", err)
			return //c.String(http.StatusConflict, "File reading error")
		}
		//fmt.Println("Contents of file:", string(data))

		cpuJSON := string(data)
		var newCPUData cpuData
		json.Unmarshal([]byte(cpuJSON), &newCPUData)
		newCPUData.Per = ((newCPUData.Idle * 100.0) / newCPUData.Total)
		newCPUData.Tiempo = currentTime.Format("2006.01.02 15:04:05")
		client.Trigger("cpuPercentage", "addNumber", newCPUData)
	}, 2500, true)

	return c.String(http.StatusOK, "CPU begun")
}

// Template estructura
type Template struct {
	templates *template.Template
}

// Render del template
func (t *Template) Render(w io.Writer, name string, data interface{}, c echo.Context) error {
	return t.templates.ExecuteTemplate(w, name, data)
}

func listarProcs(c echo.Context) error {
	data, err := ioutil.ReadFile("/proc/201503393_procs")
	if err != nil {
		fmt.Println("File reading error", err)
		return c.String(http.StatusConflict, "File reading error")
	}
	//fmt.Println("Contents of file:", string(data))

	procsJSON := string(data)
	var newProcData procsData
	json.Unmarshal([]byte(procsJSON), &newProcData)

	return c.Render(http.StatusOK, "procs", newProcData)
}

func listarCPU(c echo.Context) error {
	return c.Render(http.StatusOK, "cpu", "")
}

func listarRAM(c echo.Context) error {
	return c.Render(http.StatusOK, "ram", "")
}

func iniciar(c echo.Context) error {
	return c.Redirect(http.StatusMovedPermanently, "/procs")
}

func main() {
	t := &Template{
		templates: template.Must(template.ParseGlob("templates/*.html")),
	}

	// Echo instance
	e := echo.New()

	// Middleware
	e.Use(middleware.LoggerWithConfig(middleware.LoggerConfig{
		Format: "time=${time_rfc3339}, method=${method}, uri=${uri}, status=${status}\n",
	}))
	e.Use(middleware.Recover())

	e.Renderer = t

	// Define the HTTP routes
	e.Static("/static", "static")
	e.GET("/", iniciar)
	e.GET("/procs", listarProcs)
	e.GET("/cpu", listarCPU)
	e.GET("/ram", listarRAM)
	e.GET("/obtenerCPU", obtenerCPU)
	e.GET("/obtenerRAM", obtenerRAM)
	e.POST("/killProc", killProc)

	// Start server
	e.Logger.Fatal(e.Start(":3001"))
}

func killProc(c echo.Context) error {
	fmt.Println("matara un proc")
	name := c.FormValue("btnkill")
	fmt.Println(name)

	processes, _ := process.Processes()
	for _, proc := range processes {
		pid := fmt.Sprint(proc.Pid)
		//fmt.Println(pid)
		if pid == name {
			fmt.Println("son iguales")
			proc.Kill()
			break
		}
	}
	//return c.String(http.StatusOK, name)
	return c.Redirect(http.StatusMovedPermanently, "procs")
}
