/** @type {import('next').NextConfig} */
const nextConfig = {
  output: "export", // CRITICAL: This enables static export
  // Optional: Forces assets to be relative for serving from a non-root folder (like the ESP32)
  distDir: "out", // The static files will be placed here
  basePath: "",
  images: { unoptimized: true }, // Disable image optimization for static export
};
export default nextConfig;
