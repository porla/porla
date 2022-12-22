import React from 'react';
import ReactDOM from 'react-dom/client';
import { BrowserRouter, Routes, Route, } from 'react-router-dom';
import { ChakraProvider } from '@chakra-ui/react';

import App from './App';
import theme from './theme';
import Home from './pages/Home';
import Setup from './pages/Setup';
import { AuthProvider } from './contexts/auth';
import Login from './pages/Login';
import { NinjaProvider } from './contexts/ninja';
import { TorrentsFilterProvider } from './contexts/TorrentsFilterContext';

const basename = (window as any).porla.base_path;

ReactDOM.createRoot(document.getElementById('root') as HTMLElement).render(
  <React.StrictMode>
    <ChakraProvider theme={theme}>
        <BrowserRouter basename={basename}>
          <AuthProvider>
            <NinjaProvider>
              <TorrentsFilterProvider>
                <Routes>
                  <Route path="/" element={<App />}>
                    <Route index element={<Home />} />
                  </Route>
                  <Route path="/login" element={<Login />} />
                  <Route path="/setup" element={<Setup />} />
                </Routes>
              </TorrentsFilterProvider>
            </NinjaProvider>
          </AuthProvider>
        </BrowserRouter>
    </ChakraProvider>
  </React.StrictMode>
)
