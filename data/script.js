const toColor = number => ('#' + ('000000' + number.toString(16)).substr(-6));
const fnCatch = () => ({ colors: [0x0000ff, 0x00ff00, 0xff0000]});

fetch('/colors/')
  .then(res=>res.json())
  .catch(fnCatch)
  .then(({ colors }) => {
    const body = document.querySelector('body');
    nodes = colors.map((color, colorIndex) => {
      const node = document.createElement('input');
      body.appendChild(node);

      $(node)
        .spectrum({
          type: 'flat',
          showPalette: false,
          togglePaletteOnly: true,
          hideAfterPaletteSelect: true,
          showAlpha: false,
          showButtons: false,
          allowEmpty: false,
          color: toColor(color),
          move: throttle(100, false, (color) => {
            fetch(`/color${colorIndex}/${color.toHex()}`)
              .then(res=>res.json())
              .catch(fnCatch)
          })
        });
      return node;
    });
  });