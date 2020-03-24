document.getElementById("getUpdates").addEventListener("click", function(){
    axios.get('/obtenerCPU')
})

const pusher = new Pusher('9afe9110eb9f886eed9e', {
    cluster: 'us2',
    encrypted: true
});

const channel = pusher.subscribe('cpuPercentage');

channel.bind('addNumber', data => {
  if (newLineChart.data.labels.length > 15) {
    newLineChart.data.labels.shift();  
    newLineChart.data.datasets[0].data.shift();
  }

  newLineChart.data.labels.push(data.Tiempo);
  newLineChart.data.datasets[0].data.push(data.Per);
  newLineChart.update();

  document.getElementById('visitorCount').textContent = data.Per + "%";
});

function renderChart(cpuData) {
  var ctx = document.getElementById("realtimeChart").getContext("2d");

  var options = {};

  newLineChart = new Chart(ctx, {
    type: "line",
    data: cpuData,
    options: options
  });
}

var chartConfig = {
  labels: [],
  datasets: [
     {
        label: "Porcentaje CPU utilizado",
        fill: false,
        lineTension: 0.1,
        backgroundColor: "rgba(75,192,192,0.4)",
        borderColor: "rgba(75,192,192,1)",
        borderCapStyle: 'butt',
        borderDash: [],
        borderDashOffset: 0.0,
        borderJoinStyle: 'miter',
        pointBorderColor: "rgba(75,192,192,1)",
        pointBackgroundColor: "#fff",
        pointBorderWidth: 1,
        pointHoverRadius: 5,
        pointHoverBackgroundColor: "rgba(75,192,192,1)",
        pointHoverBorderColor: "rgba(220,220,220,1)",
        pointHoverBorderWidth: 2,
        pointRadius: 1,
        pointHitRadius: 10,
        data: [],
        spanGaps: false,
     }
  ]
};

renderChart(chartConfig)