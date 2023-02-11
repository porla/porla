import { createContext, ReactNode, useContext, useMemo, useState } from "react"
import { TorrentsListFilters } from "../types";

type TorrentsFilterContextType = {
  clearFilter: (field: keyof TorrentsListFilters) => void;
  clearFilters: () => void;
  filters: TorrentsListFilters;
  setFilter<K extends keyof TorrentsListFilters>(key: K, value: TorrentsListFilters[K]): void;
}

const TorrentsFilterContext = createContext<TorrentsFilterContextType>({} as TorrentsFilterContextType);

export function TorrentsFilterProvider({ children } : { children: ReactNode }): JSX.Element {
  const [filters, set_f] = useState<TorrentsListFilters>({});

  function clearFilter(field: keyof TorrentsListFilters) {
    set_f(f => {
      let n = { ...f };
      delete n[field];
      return n;
    });
  }

  function clearFilters() {
    set_f({});
  }

  function setFilter<K extends keyof TorrentsListFilters>(key: K, value: TorrentsListFilters[K]) {
    set_f(f => {
      let n = { ...f };
      n[key] = value
      return n;
    });
  }

  const memoedValue = useMemo(() => ({
    setFilter,
    clearFilter,
    clearFilters,
    filters,
  }), [filters]);

  return (
    <TorrentsFilterContext.Provider value={memoedValue}>
      {children}
    </TorrentsFilterContext.Provider>
  );
}

export default function useTorrentsFilter() {
  return useContext(TorrentsFilterContext);
}
