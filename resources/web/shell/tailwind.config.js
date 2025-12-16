export default {
  darkMode: ['selector', '[data-theme="dark"]'],
  content: ['./index.html', './src/**/*.{vue,js,ts,jsx,tsx}'],
  theme: {
    extend: {
      colors: {
        navbar: {
          bg: '#222224',
          active: '#009688',
          hover: '#333336',
          text: '#ffffff',
          'text-secondary': '#a0a0a0',
          border: '#1a1a1c',
        }
      }
    }
  },
  plugins: [],
}
