let nodes = null;

const toColor = number => ('#' + ('000000' + number.toString(16)).substr(-6));

const query = (name, value='') => {
  return fetch(`/${name}/${value.substr(1)}`)
    .then(res=>res.json())
    .then(data => {
      if (nodes) {
        data.colors.forEach((color, index) => {
          nodes[index].value = toColor(color);
        });
      } else {
        return data.colors;
      }
    });    
};

query('colors')
  .then(colors => {
    const body = document.querySelector('body');
    nodes = colors.map((color, index) => {
      const node = document.createElement('input');
      node.type = 'color';
      node.name = `color${index}`;
      node.value = toColor(color);
      node.addEventListener('change', ({target}) => {
        query(target.name, target.value);
      });          
      body.appendChild(node);
      return node;
    });
  });