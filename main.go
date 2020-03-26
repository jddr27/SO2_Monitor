package main

import (
	"encoding/json"
	"fmt"
	"html/template"
	"io"
	"io/ioutil"
	"net/http"
	"strconv"
	"time"

	"github.com/labstack/echo"
	"github.com/labstack/echo/middleware"

	pusher "github.com/pusher/pusher-http-go"
	"github.com/shirou/gopsutil/cpu"
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

type cpuData struct {
	Per    float64
	Tiempo string
}

type ramData struct {
	Total  uint64
	Usado  uint64
	Per    float64
	Tiempo string
}

type procsData struct {
	Pid    int32
	Usu    string
	Est    string
	Per    float32
	Nombre string
}

type procInfo struct {
	Total int
	Cor   int
	Sus   int
	Det   int
	Zom   int
}

type procFinal struct {
	Lista []procsData
	Info  procInfo
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
			return c.String(http.StatusConflict, "File reading error")
		}
		fmt.Println("Contents of file:", string(data))

		ramJSON := string(data)
		var ramData newRAMData
		json.Unmarshal([]byte(ramJSON), &newRAMData)
		newRAMData.Tiempo = currentTime.Format("2006.01.02 15:04:05")

		client.Trigger("ramPercentage", "addNumber", newRAMData)
	}, 2500, true)

	return c.String(http.StatusOK, "RAM begun")
}

func obtenerCPU(c echo.Context) error {
	setInterval(func() {
		currentTime := time.Now()
		percentage, _ := cpu.Percent(0, true)
		newCPUData := cpuData{
			Per:    percentage[0],
			Tiempo: currentTime.Format("2006.01.02 15:04:05"),
		}
		//fmt.Println(percentage[0])
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
	arrayProcs := []procsData{}
	total := 0
	pR := 0
	pS := 0
	pT := 0
	pZ := 0
	processes, _ := process.Processes()

	for _, proc := range processes {
		pid := proc.Pid
		username, _ := proc.Username()
		estado, _ := proc.Status()
		porcentaje, _ := proc.MemoryPercent()
		nombre, _ := proc.Name()
		newProcsData := procsData{
			Pid:    pid,
			Usu:    username,
			Est:    estado,
			Per:    porcentaje,
			Nombre: nombre,
		}
		//fmt.Println(newProcsData.Pid, newProcsData.Usu, newProcsData.Est, newProcsData.Per, newProcsData.Nombre)
		arrayProcs = append(arrayProcs, newProcsData)
		total = total + 1
		switch estado {
		case "R":
			pR = pR + 1
		case "S":
			pS = pS + 1
		case "T":
			pT = pT + 1
		case "Z":
			pZ = pZ + 1
		}
	}
	//for _, elem := range arrayProcs {
	//	fmt.Println(elem.Pid, elem.Usu, elem.Est, elem.Per, elem.Nombre)
	//}
	newProcInfo := procInfo{
		Total: total,
		Cor:   pR,
		Sus:   pS,
		Det:   pT,
		Zom:   pZ,
	}
	//fmt.Println(newProcInfo.Total, newProcInfo.Cor, newProcInfo.Sus, newProcInfo.Det, newProcInfo.Zom)
	//fmt.Println("entro")

	newProcFinal := procFinal{
		Lista: arrayProcs,
		Info:  newProcInfo,
	}

	return c.Render(http.StatusOK, "procs", newProcFinal)
}

func listarCPU(c echo.Context) error {
	percentage, _ := cpu.Percent(0, true)
	texto := ""
	for _, cpupercent := range percentage {
		texto = texto + strconv.FormatFloat(cpupercent, 'f', 2, 64) + "%"
	}
	return c.Render(http.StatusOK, "cpu", texto)
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
