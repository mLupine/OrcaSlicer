import { createApp } from 'vue'
import { createPinia } from 'pinia'
import App from './App.vue'
import './style.css'
import { useNavigationStore } from './stores/navigation'
import { useAppStore } from './stores/app'
import { useIPC } from './composables/useIPC'
import type { NavbarAppState } from './types/ipc'

const pinia = createPinia()
const app = createApp(App)

app.use(pinia)
app.mount('#app')

window.updateAppState = (state: Partial<NavbarAppState>) => {
  const navigationStore = useNavigationStore()
  const appStore = useAppStore()

  navigationStore.updateState({
    activeTab: state.activeTabId,
    tabVisibility: state.tabVisibility,
  })

  appStore.updateState({
    title: state.title,
    hasUnsavedChanges: state.hasUnsavedChanges,
    canUndo: state.canUndo,
    canRedo: state.canRedo,
    canSave: state.canSave,
  })
}

const { send } = useIPC()
send('getState')
  .then((response) => {
    if (response?.success && response.state && window.updateAppState) {
      window.updateAppState(response.state as Partial<NavbarAppState>)
    }
  })
  .catch(() => {
  })
