import React from "react";

export default function Porla(props: any) {
  return (
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 125 125" width={props.width} height={props.height}>
      <defs>
        <linearGradient id="grad" x1="18.774" y1="107.162" x2="107.098" y2="18.839" gradientUnits="userSpaceOnUse">
          <stop offset="0.000" stopColor="#6f23be" />
          <stop offset="0.178" stopColor="#6c27bf" />
          <stop offset="0.363" stopColor="#6334c1" />
          <stop offset="0.551" stopColor="#5348c6" />
          <stop offset="0.741" stopColor="#3e65cc" />
          <stop offset="0.930" stopColor="#238ad4" />
          <stop offset="1.000" stopColor="#179ad7" />
        </linearGradient>
      </defs>
      <path fill="url(#grad)" d="M32.77,113.62A22.83,22.83,0,0,0,35,117.69c.25.44.51.88.79,1.3A62.47,62.47,0,0,1,0,62.5c0-.76,0-1.52,0-2.28a12.45,12.45,0,0,1,12.4-11.89l31.29-.22A14.87,14.87,0,0,1,52.47,50c9.3,5.3,10.63,18.21,3.46,26a19,19,0,0,1-8.55,5.46C34.77,86.2,27.45,101,32.77,113.62ZM64.78,0A62.25,62.25,0,0,0,29,9.75a6.25,6.25,0,0,0,3.26,11.56l11.2.08a40.58,40.58,0,0,1,15.86,3.72C72.8,31.23,82.17,45.43,82.17,60.2A37.24,37.24,0,0,1,51.82,97.43c-5.48,1.51-7.72,7.24-7.1,13a14.06,14.06,0,0,0,4.48,9A21.73,21.73,0,0,0,62.8,125a62.5,62.5,0,0,0,2-125Z" />
    </svg>
  );
}
