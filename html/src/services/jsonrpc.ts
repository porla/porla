import useSWR from "swr";

const fetcher = function<T>() {
  return async (method: string, params: any, ...args: any[]) => {
    return await jsonrpc<T>(method, params);
  };
}

export async function jsonrpc<T>(method: string, params?: any) {
  const res = await fetch('/api/v1/jsonrpc', {
    body: JSON.stringify({
      jsonrpc: '2.0',
      method,
      params: params || {}
    }),
    method: 'POST'
  });

  if (res.status !== 200) {
    throw new Error(res.statusText);
  }

  const data = await res.json();

  if (data.error) {
    throw new Error(data.error.message);
  }

  return data.result as T;
}

export function useRPC<T>(method: string, params?: any, config?: any) {
  return useSWR([method, params], fetcher<T>(), config);
}
