import { createContext, ReactNode, useContext, useMemo, useState } from "react"

type NinjaContextType = {
  isNinja: boolean;
  toggleNinja: () => void;
}

const NinjaContext = createContext<NinjaContextType>({} as NinjaContextType);

export function NinjaProvider({ children } : { children: ReactNode }): JSX.Element {
  const [isNinja, setIsNinja] = useState<boolean>(false);

  function toggleNinja() {
    setIsNinja(!isNinja);
  }

  const memoedValue = useMemo(() => ({
    isNinja,
    toggleNinja
  }), [isNinja]);

  return (
    <NinjaContext.Provider value={memoedValue}>
      {children}
    </NinjaContext.Provider>
  );
}

export default function useNinja() {
  return useContext(NinjaContext);
}
