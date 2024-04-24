function showTime() {
  var date = new Date();
  document.getElementById("MyClockDisplay").innerText = date.toLocaleString();
  document.getElementById("MyClockDisplay").textContent = date.toLocaleString();
  setTimeout(showTime, 1000);
}
showTime();

const xValues = [10000, 200, 300, 400, 500, 600];

new Chart("myChart", {
  type: "line",
  data: {
    labels: xValues,
    datasets: [
      {
        data: [860, 1140, 1060, 1060, 1070, 1110],
        borderColor: "red",
        fill: false,
      },
    ],
  },
  options: {
    legend: { display: false },
  },
});
