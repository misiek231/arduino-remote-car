module.exports = {
  proxy: "http://localhost:3000",
  files: ["public/**/*.{html,ts,css}", "src/**/*.ts"],
  port: 3001,
  open: false,
  notify: true,
  reloadDelay: 5000 // Add a slight delay to ensure the server restarts before reloading
};