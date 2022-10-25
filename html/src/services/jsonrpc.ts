import useSWR from "swr";

const fetcher = function<T>() {
  return async (method: string, init: RequestInit, ...args: any[]) => {
    const res = await fetch('/api/v1/jsonrpc', {
      body: JSON.stringify({
        jsonrpc: '2.0',
        method,
        params: {}
      }),
      method: 'POST'
    });

    const data = await res.json();

    return data.result as T;
  };
}

export default function jsonrpc<T>(method: string, params?: any) {
  const { error, data } = useSWR([method, params], fetcher<T>(), { refreshInterval: 1000 });
  return {error,data};
}
