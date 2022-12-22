import { createContext, ReactNode, useContext, useMemo, useState } from "react"
import { Filter } from "../types";

type TorrentsFilterContextType = {
  addFilter: (filter: Filter) => void;
  clearFilter: (field: string) => void;
  clearFilters: () => void;
  filters: Filter[];
}

const TorrentsFilterContext = createContext<TorrentsFilterContextType>({} as TorrentsFilterContextType);

export function TorrentsFilterProvider({ children } : { children: ReactNode }): JSX.Element {
  const [filters, setFilters] = useState<Filter[]>([]);

  function addFilter(filter: Filter) {
    setFilters(() => [filter, ...filters])
  }

  function clearFilter(field: string) {
    setFilters(() => [...filters.filter(f => f.field !== field)])
  }

  function clearFilters() {
    setFilters(() => []);
  }

  const memoedValue = useMemo(() => ({
    addFilter,
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
