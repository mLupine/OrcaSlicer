import { defineStore } from 'pinia'
import { ref, computed } from 'vue'

export interface Tab {
  id: string
  label: string
  icon: string
  visible: boolean
}

export const useNavigationStore = defineStore('navigation', () => {
  const activeTab = ref('prepare')

  const tabs = ref<Tab[]>([
    { id: 'home', label: 'Home', icon: 'home', visible: true },
    { id: 'prepare', label: 'Prepare', icon: 'box', visible: true },
    { id: 'preview', label: 'Preview', icon: 'eye', visible: true },
    { id: 'device', label: 'Device', icon: 'printer', visible: true },
    { id: 'multi-device', label: 'Multi-device', icon: 'layoutGrid', visible: true },
    { id: 'project', label: 'Project', icon: 'folderOpen', visible: true },
    { id: 'calibration', label: 'Calibration', icon: 'settings2', visible: true },
    { id: 'auxiliary', label: 'Auxiliary', icon: 'settings2', visible: false },
    { id: 'debug-tool', label: 'Debug Tool', icon: 'settings2', visible: false },
  ])

  const visibleTabs = computed(() => tabs.value.filter(tab => tab.visible))

  const setActiveTab = (tabId: string) => {
    const tab = tabs.value.find(t => t.id === tabId)
    if (tab && tab.visible) {
      activeTab.value = tabId
    }
  }

  const updateTabVisibility = (tabId: string, visible: boolean) => {
    const tab = tabs.value.find(t => t.id === tabId)
    if (tab) {
      tab.visible = visible
    }
  }

  const updateState = (state: { activeTab?: string; tabVisibility?: Record<string, boolean> }) => {
    if (state.activeTab) {
      setActiveTab(state.activeTab)
    }
    if (state.tabVisibility) {
      Object.entries(state.tabVisibility).forEach(([tabId, visible]) => {
        updateTabVisibility(tabId, visible)
      })
    }
  }

  return {
    activeTab,
    tabs,
    visibleTabs,
    setActiveTab,
    updateTabVisibility,
    updateState,
  }
})
