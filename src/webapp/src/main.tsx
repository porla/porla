import { ChakraProvider } from "@chakra-ui/react";
import React from "react";
import ReactDOM from "react-dom/client";
import { BrowserRouter, Routes, Route, } from "react-router-dom";

import App from "./App";

import "./scss/main.scss";
import Add from "./pages/Add";
import Torrents from "./pages/Torrents";
import Settings from "./pages/Settings";
import Home from "./pages/Home";

ReactDOM.createRoot(document.getElementById("root") as HTMLElement).render(
  <React.StrictMode>
    <ChakraProvider>
      <BrowserRouter>
        <Routes>
          <Route path="/" element={<App />}>
            <Route index element={<Home />} />
            <Route path="torrents" element={<Torrents />} />
            <Route path="torrents/add" element={<Add />} />
            <Route path="settings" element={<Settings />} />
          </Route>
        </Routes>
      </BrowserRouter>
    </ChakraProvider>
  </React.StrictMode>
);
