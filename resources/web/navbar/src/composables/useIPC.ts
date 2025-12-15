import type { IPCMessage } from '../types/ipc'

export function useIPC() {
  const send = async (type: string, payload: Record<string, unknown> = {}): Promise<any> => {
    return new Promise((resolve, reject) => {
      if (!window.cefQuery) {
        console.log('IPC Mock:', type, payload)
        resolve({ success: true })
        return
      }

      const message: IPCMessage = { type, payload }

      window.cefQuery({
        request: JSON.stringify(message),
        onSuccess: (response: string) => {
          try {
            resolve(JSON.parse(response))
          } catch {
            resolve(response)
          }
        },
        onFailure: (errorCode: number, errorMessage: string) => {
          reject(new Error(`IPC Error ${errorCode}: ${errorMessage}`))
        }
      })
    })
  }

  return { send }
}
