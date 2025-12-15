# Quick Start Guide

Get the OrcaSlicer navigation bar running in 2 minutes.

## Install Dependencies

```bash
cd /Users/maciej/Code/.worktrees/OrcaSlicer/infill/resources/web/navbar
bun install
```

## Development Mode

```bash
bun run dev
```

Open http://localhost:5173 in your browser. The app will hot-reload on changes.

## Build for Production

```bash
bun run build
```

Output: `dist/` directory ready for CEF integration.

## Testing IPC Communication

### In Browser (Development)

Open browser console. IPC messages will be logged:

```javascript
IPC Mock: tab_selected { tabId: 'prepare' }
IPC Mock: file_menu_action { action: 'open' }
```

### In CEF (Production)

The C++ side receives messages via `window.cefQuery()`:

```json
{
  "type": "tab_selected",
  "payload": { "tabId": "prepare" }
}
```

### Update State from C++

```javascript
window.updateNavbarState({
  activeTab: 'preview',
  title: 'my_model.3mf',
  canUndo: true,
  canRedo: false
})
```

## Project Structure at a Glance

```
src/
├── components/      # UI components
├── composables/     # Reusable logic (IPC, navigation)
├── stores/          # Pinia state (navigation, app)
├── types/           # TypeScript interfaces
├── main.ts          # Entry point
└── App.vue          # Root component
```

## Key Files to Customize

- **src/stores/navigation.ts** - Tab configuration
- **src/stores/app.ts** - App state defaults
- **tailwind.config.js** - Theme colors
- **src/components/Navbar.vue** - Layout

## Available Commands

```bash
bun run dev      # Development server (HMR enabled)
bun run build    # Production build
bun run preview  # Preview production build locally
```

## Common Tasks

### Add a New Tab

Edit `src/stores/navigation.ts`:

```typescript
const tabs = ref<Tab[]>([
  { id: 'prepare', label: 'Prepare', icon: 'box', visible: true },
  { id: 'preview', label: 'Preview', icon: 'eye', visible: true },
  { id: 'device', label: 'Device', icon: 'printer', visible: true },
  { id: 'calibrate', label: 'Calibrate', icon: 'settings', visible: true }, // New!
])
```

### Add a New Menu Item

Edit `src/components/FileMenu.vue`:

```typescript
const menuItems = [
  { id: 'new', label: 'New Project' },
  { id: 'open', label: 'Open...' },
  // ... existing items ...
  { id: 'export-stl', label: 'Export STL...' }, // New!
]
```

### Change Colors

Edit `tailwind.config.js`:

```javascript
colors: {
  navbar: {
    bg: '#1a1f20',        // Darker background
    active: '#ff6b35',    // Orange active state
    hover: '#2a2f30',     // Lighter hover
  }
}
```

## Troubleshooting

### Port 5173 Already in Use

```bash
pkill -f "vite"
bun run dev
```

### Build Fails

```bash
rm -rf node_modules bun.lockb
bun install
bun run build
```

### TypeScript Errors

Check `tsconfig.json` and ensure all `.d.ts` files are included.

## Next Steps

1. Customize components in `src/components/`
2. Add your IPC message handlers in `src/composables/useIPC.ts`
3. Update tab configuration in `src/stores/navigation.ts`
4. Build and integrate into CEF

---

For full documentation, see README.md
