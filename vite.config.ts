import { defineConfig } from "vite";
import react from "@vitejs/plugin-react";

// https://vitejs.dev/config/
export default defineConfig({
  build: {
    outDir: "../../dist/html"
  },
  plugins: [react()],
  root: "./src/webapp"
});
