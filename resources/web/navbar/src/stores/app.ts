import { defineStore } from 'pinia'
import { ref } from 'vue'

export const useAppStore = defineStore('app', () => {
  const title = ref('OrcaSlicer')
  const hasUnsavedChanges = ref(false)
  const justSaved = ref(false)
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
    if (state.hasUnsavedChanges !== undefined) {
      const wasUnsaved = hasUnsavedChanges.value
      hasUnsavedChanges.value = state.hasUnsavedChanges
      if (wasUnsaved && !state.hasUnsavedChanges) {
        justSaved.value = true
      }
    }
    if (state.canUndo !== undefined) canUndo.value = state.canUndo
    if (state.canRedo !== undefined) canRedo.value = state.canRedo
    if (state.canSave !== undefined) canSave.value = state.canSave
  }

  const clearJustSaved = () => {
    justSaved.value = false
  }

  return {
    title,
    hasUnsavedChanges,
    justSaved,
    canUndo,
    canRedo,
    canSave,
    updateState,
    clearJustSaved,
  }
})
