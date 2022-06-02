const queryString = window.location.search;
const urlParams = new URLSearchParams(queryString);
const id = urlParams.get('id');

fetch(`http://18.188.228.61:5000/data?id=${id}`)
  .then(res => res.json())
  .then(resData => {
    console.log(resData);
    const labels = resData.result.map(e => {
      const time = parseInt(e.timestamp);
      const date = new Date(0);
      date.setUTCSeconds(time);
      return date.toUTCString();
    });

    const data = {
      labels: labels,
      datasets: [{
        label: 'Charge for ID: ',
        backgroundColor: 'rgb(255, 99, 132)',
        borderColor: 'rgb(255, 99, 132)',
        data: resData.result.map(e => e.charge),
      }]
    };

    const config = {
      type: 'line',
      data: data,
      options: {}
    };

    const myChart = new Chart(
      document.getElementById('myChart'),
      config
    );
  });
