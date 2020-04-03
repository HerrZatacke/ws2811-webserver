const toColor = number => ('#' + ('000000' + number.toString(16)).substr(-6));
const fnCatch = () => ({ colors: [0x0000ff, 0x00ff00, 0xff0000]});

const createColorpicker = (color, colorName) => {
  $('<input/>')
    .appendTo('body')
    .spectrum({
      type: 'flat',
      showPalette: false,
      togglePaletteOnly: true,
      hideAfterPaletteSelect: true,
      showAlpha: false,
      showButtons: false,
      allowEmpty: false,
      color,
      move: throttle(25, false, (color) => {
        fetch(`/${colorName}/${color.toHex()}`)
          .then(res=>res.json())
          .catch(fnCatch)
      })
    });
}

fetch('/colors/')
  .then(res=>res.json())
  .catch(fnCatch)
  .then(({ colors }) => colors.map(color => {
    const hex = toColor(color);
    const tiny = tinycolor(hex);
    return {
      hex,
      tiny,
      r: tiny._r,
      g: tiny._g,
      b: tiny._b,
    };
  }))
  .then((colors) => {
    console.log(colors);
    if (colors.length) {
      colors.forEach(({ hex }, index) => createColorpicker(hex, `color${index}`));
      
      const average = colors.reduce(({ ir, ig, ib }, { r, g, b }) => {
        return {
          ir: ir + r,
          ig: ig + g, 
          ib: ib + b,
        };
      }, { ir: 0, ig: 0, ib: 0 });

      const averageRgb = tinycolor(`rgb(${Math.ceil(average.ir / colors.length)},${Math.ceil(average.ig / colors.length)},${Math.ceil(average.ib / colors.length)})`).toHexString();
      $('<hr/>').appendTo('body');
      createColorpicker(averageRgb, 'colorAll');
    }
  });