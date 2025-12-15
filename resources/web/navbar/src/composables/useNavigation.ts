import { useNavigationStore } from '../stores/navigation'
import { useIPC } from './useIPC'

export function useNavigation() {
  const store = useNavigationStore()
  const { send } = useIPC()

  const selectTab = async (tabId: string) => {
    store.setActiveTab(tabId)
    await send('selectTab', { tabId })
  }

  const isTabVisible = (tabId: string): boolean => {
    const tab = store.tabs.find(t => t.id === tabId)
    return tab?.visible ?? false
  }

  const getActiveTab = () => {
    return store.tabs.find(t => t.id === store.activeTab)
  }

  return {
    selectTab,
    isTabVisible,
    getActiveTab,
    activeTab: () => store.activeTab,
    tabs: () => store.tabs,
  }
}
