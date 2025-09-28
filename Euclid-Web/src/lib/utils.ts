import { clsx, type ClassValue } from "clsx"
import { twMerge } from "tailwind-merge"

export function cn(...inputs: ClassValue[]) {
  return twMerge(clsx(inputs))
}

export function generateGradient() {
  const colors = [
    "#ff9a9e",
    "#fad0c4",
    "#fad0c4",
    "#fbc2eb",
    "#a18cd1",
    "#fbc2eb",
    "#a6c1ee",
    "#667eea",
    "#764ba2",
    "#89f7fe",
    "#66a6ff",
    "#43e97b",
    "#38f9d7",
    "#30cfd0",
    "#330867",
  ];
  const angle = Math.floor(Math.random() * 360);
  const color1 = colors[Math.floor(Math.random() * colors.length)];
  let color2 = colors[Math.floor(Math.random() * colors.length)];
  while (color1 === color2) {
    color2 = colors[Math.floor(Math.random() * colors.length)];
  }
  return `linear-gradient(${angle}deg, ${color1}, ${color2})`;
}
