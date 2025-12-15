import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useAppStore = defineStore('app', () => {
  const title = ref('OrcaSlicer')
  const hasUnsavedChanges = ref(false)
  const canUndo = ref(false)
  const canRedo = ref(false)
  const canSave = ref(false)

  const updateState = (state: {
    title?: string
    hasUnsavedChanges?: boolean
    canUndo?: boolean
    canRedo?: boolean
    canSave?: boolean
  }) => {
    if (state.title !== undefined) title.value = state.title
    if (state.hasUnsavedChanges !== undefined) hasUnsavedChanges.value = state.hasUnsavedChanges
    if (state.canUndo !== undefined) canUndo.value = state.canUndo
    if (state.canRedo !== undefined) canRedo.value = state.canRedo
    if (state.canSave !== undefined) canSave.value = state.canSave
  }

  return {
    title,
    hasUnsavedChanges,
    canUndo,
    canRedo,
    canSave,
    updateState,
  }
})
