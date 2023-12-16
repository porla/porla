export function applyRatioColor (ratio: number) {
  const colorMap = {
    'text-amber-200 drop-shadow-[0_0_5px_rgba(255,255,0,1)]': 100,
    'text-orange-500': 50,
    'text-purple-500': 10,
    'text-blue-500': 5,
    'text-green-500': 2,
    'text-neutral-300': 1
  };

  const colors = Object.keys(colorMap);
  const ratios = Object.values(colorMap);

  for (let i = 0; i < ratios.length; i++) {
    if (ratio >= ratios[i]) {
      return colors[i];
    }
  }

  return 'text-neutral-500';

}