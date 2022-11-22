import prefixPath from "../base";

export default {
  init: async (username: string, password: string) => {
    const res = await fetch(prefixPath("/api/v1/auth/init"), {
      body: JSON.stringify({
        username,
        password
      }),
      method: "POST"
    });

    if (res.status !== 200) {
      throw new Error(res.statusText);
    }

    return await res.json();
  },

  login: async (username: string, password: string): Promise<string> => {
    const res = await fetch(prefixPath("/api/v1/auth/login"), {
      body: JSON.stringify({
        username,
        password
      }),
      method: "POST"
    });

    if (res.status !== 200) {
      throw new Error(res.statusText);
    }

    const { token } = await res.json();

    return token;
  }
};
