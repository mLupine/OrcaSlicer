# OrcaSlicer Navigation Bar

Vue 3 + TypeScript + Tailwind CSS application for the OrcaSlicer navigation bar UI.

## Technology Stack

- **Vue 3** with Composition API and `<script setup>`
- **TypeScript** for type safety
- **Pinia** for state management
- **Tailwind CSS** for styling
- **Lucide Vue** for icons
- **Vite** for build tooling
- **Bun** for package management

## Project Structure

```
navbar/
├── src/
│   ├── components/        # Vue components
│   │   ├── Navbar.vue    # Main navigation bar container
│   │   ├── TabButton.vue # Tab navigation button
│   │   ├── IconButton.vue # Generic icon button
│   │   ├── FileMenu.vue  # File dropdown menu
│   │   ├── DropdownMenu.vue # Settings/more menu
│   │   └── WindowControls.vue # Window minimize/maximize/close
│   ├── composables/       # Reusable composition functions
│   │   ├── useIPC.ts     # CEF IPC communication wrapper
│   │   └── useNavigation.ts # Navigation logic
│   ├── stores/           # Pinia state stores
│   │   ├── navigation.ts # Tab navigation state
│   │   └── app.ts        # Application state
│   ├── types/            # TypeScript type definitions
│   │   └── ipc.ts        # IPC message types
│   ├── App.vue           # Root component
│   ├── main.ts           # Application entry point
│   └── style.css         # Global styles and Tailwind imports
├── dist/                 # Build output (generated)
└── index.html           # HTML entry point
```

## Development

### Prerequisites

- Bun (latest version)

### Install Dependencies

```bash
bun install
```

### Development Server

```bash
bun run dev
```

The app will be available at http://localhost:5173

### Build for Production

```bash
bun run build
```

Output will be in the `dist/` directory.

### Type Check

```bash
bun run build
```

TypeScript checking is integrated into the build process.

## CEF Integration

### IPC Communication

The app communicates with C++ via the `window.cefQuery()` API. All IPC messages follow this structure:

```typescript
interface IPCMessage {
  type: string
  payload: Record<string, unknown>
}
```

### Sending Messages to C++

```typescript
import { useIPC } from './composables/useIPC'

const { send } = useIPC()

await send('tab_selected', { tabId: 'prepare' })
await send('file_menu_action', { action: 'open' })
```

### Receiving Messages from C++

The C++ side can update the navbar state by calling:

```javascript
window.updateNavbarState({
  activeTab: 'preview',
  title: 'My Project',
  canUndo: true,
  canRedo: false,
  canPublish: true,
  tabVisibility: {
    prepare: true,
    preview: true,
    device: false
  }
})
```

## Available IPC Messages

### From Vue to C++

- `tab_selected` - User clicked a tab
  - Payload: `{ tabId: string }`
- `file_menu_action` - File menu item clicked
  - Payload: `{ action: 'new' | 'open' | 'save' | 'save-as' | 'import' | 'export' }`
- `menu_action` - Settings menu item clicked
  - Payload: `{ action: 'preferences' | 'publish' | 'help' | 'about' }`
- `undo` - Undo button clicked
- `redo` - Redo button clicked
- `window_minimize` - Minimize window (Windows/Linux)
- `window_maximize` - Maximize/restore window (Windows/Linux)
- `window_close` - Close window (Windows/Linux)

### From C++ to Vue

Use `window.updateNavbarState(state)` with these properties:

- `activeTab: string` - Currently active tab ID
- `title: string` - Window title
- `canUndo: boolean` - Enable/disable undo button
- `canRedo: boolean` - Enable/disable redo button
- `canPublish: boolean` - Enable/disable publish menu item
- `tabVisibility: Record<string, boolean>` - Show/hide tabs

## Component Architecture

### State Management

The application uses Pinia stores for centralized state management:

- **Navigation Store** - Tab state and visibility
- **App Store** - Application-level state (undo/redo, publish, title)

### Composables

Reusable logic is extracted into composables:

- **useIPC** - CEF communication wrapper with dev mode fallback
- **useNavigation** - Tab navigation logic

### Component Hierarchy

```
App.vue
└── Navbar.vue
    ├── FileMenu.vue
    ├── TabButton.vue (multiple)
    ├── IconButton.vue (multiple)
    ├── DropdownMenu.vue
    │   └── IconButton.vue
    └── WindowControls.vue
```

## Styling

### Tailwind Configuration

The app uses a custom color palette for the navbar:

```javascript
colors: {
  navbar: {
    bg: '#262e30',           // Background
    active: '#009688',       // Active tab
    hover: '#374244',        // Hover state
    text: '#ffffff',         // Primary text
    'text-secondary': '#a0a0a0', // Secondary text
    border: '#1a1f20'        // Border color
  }
}
```

### Dark Mode

Dark mode is enabled by default using the `[data-theme="dark"]` selector strategy.

### Window Dragging

The navbar supports window dragging on CEF:

- `.drag-region` - Areas that allow window dragging
- `.no-drag-region` - Interactive elements (buttons, menus)

## Browser Compatibility

The app is designed for CEF (Chromium Embedded Framework) but also works standalone in modern browsers for development.

### Development Mode

When `window.cefQuery` is not available, the IPC layer logs messages to console instead of communicating with C++.

## Future Enhancements (Phase 4)

- Enhanced animations and transitions
- Keyboard shortcuts
- Accessibility improvements (ARIA labels)
- Responsive layout for different window sizes
- Context menus
- Custom themes
