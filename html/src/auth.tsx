import * as React from 'react'

export interface AuthContext {
  isAuthenticated: boolean
  login: (username: string) => void
  logout: () => void
  user: string | null
}

export const AuthContext = React.createContext<AuthContext | null>(null)

const key = 'porla.auth'

function getStoredUser() {
  return localStorage.getItem(key)
}

function setStoredUser(user: string | null) {
  if (user) {
    localStorage.setItem(key, user)
  } else {
    localStorage.removeItem(key)
  }
}

export function AuthProvider({ children }: { children: React.ReactNode }) {
  const [user, setUser] = React.useState<string | null>(getStoredUser())
  const isAuthenticated = !!user

  const logout = React.useCallback(() => {
    setStoredUser(null)
    setUser(null)
  }, [])

  const login = React.useCallback((username: string) => {
    setStoredUser(username)
    setUser(username)
  }, [])

  return (
    <AuthContext.Provider value={{ isAuthenticated, user, login, logout }}>
      {children}
    </AuthContext.Provider>
  )
}
