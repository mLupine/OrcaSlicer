export interface IPCMessage {
  type: string
  payload: Record<string, unknown>
}

export type TabId =
  | 'home'
  | 'prepare'
  | 'preview'
  | 'device'
  | 'multi-device'
  | 'project'
  | 'calibration'
  | 'auxiliary'
  | 'debug-tool'

export interface NavbarAppState {
  activeTabId: TabId
  tabVisibility: Record<TabId, boolean>
  title: string
  hasUnsavedChanges: boolean
  canUndo: boolean
  canRedo: boolean
  canSave: boolean
}

export interface CEFQueryRequest {
  request: string
  persistent?: boolean
  onSuccess: (response: string) => void
  onFailure: (errorCode: number, errorMessage: string) => void
}

declare global {
  interface Window {
    cefQuery?: (request: CEFQueryRequest) => number
    cefQueryCancel?: (id: number) => void
    updateAppState?: (state: Partial<NavbarAppState>) => void
  }
}

export {}
