import ReactDOM from 'react-dom/client'

import './index.css'
import { App } from "./App.tsx";
import React from 'react';

const rootElement = document.getElementById('root')!

if (!rootElement.innerHTML) {
  const root = ReactDOM.createRoot(rootElement)
  root.render(
    <React.StrictMode>
      <App />
    </React.StrictMode>
  )
}
