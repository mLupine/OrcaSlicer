export interface HoleBounds {
  x: number
  y: number
  width: number
  height: number
}

export interface LayoutState {
  sidebarWidth: number
  activeView: 'home' | 'prepare' | 'preview' | 'device' | 'monitor'
  holes: Record<string, HoleBounds>
}

export interface LayoutUpdateMessage {
  id: string
  bounds: HoleBounds
}
