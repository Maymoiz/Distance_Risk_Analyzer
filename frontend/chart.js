const ctx = document.getElementById("distanceChart");
let distanceData = [];

const distanceChart = new Chart(ctx, {
  type: "line",
  data: {
    labels: [],
    datasets: [{
      label: "Distance (cm)",
      data: distanceData,
      borderColor: "#a855f7",
      backgroundColor: "rgba(168,85,247,0.2)",
      tension: 0.3
    }]
  },
  options: {
    animation: { duration: 300 },
    scales: {
      x: { display: false },
      y: { beginAtZero: true }
    }
  }
});

function updateChart(value) {
  distanceChart.data.labels.push("");
  distanceChart.data.datasets[0].data.push(value);

  if (distanceChart.data.labels.length > 40) {
    distanceChart.data.labels.shift();
    distanceChart.data.datasets[0].data.shift();
  }

  distanceChart.update();
}
