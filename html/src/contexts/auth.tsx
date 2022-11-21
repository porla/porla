import { createContext, ReactNode, useContext, useMemo, useState } from "react"
import auth from "../services/auth";

type AuthContextType = {
  login: (username: string, password: string) => Promise<void>;
  token?: string;
  user?: any;
}

const AuthContext = createContext<AuthContextType>({} as AuthContextType);

export function AuthProvider({ children } : { children: ReactNode }): JSX.Element {
  const [error, setError] = useState<any>();
  const [user, setUser] = useState<any>();
  const [token, setToken] = useState<string | undefined>();

  function login(username: string, password: string) {
    return auth.login(username, password)
      .then(t => {
        setToken(t);
        setUser({ username });
      });
  }

  const memoedValue = useMemo(() => ({
    error,
    login,
    token,
    user
  }), [error, token, user]);

  return (
    <AuthContext.Provider value={memoedValue}>
      {children}
    </AuthContext.Provider>
  );
}

export default function useAuth() {
  return useContext(AuthContext);
}
