import React from "react";
import { trpc } from "../utils/trpc";

function Home() {
  const torrents = trpc.useQuery(["torrents.list"]);

  if (!torrents.data) {
    return <div>Loading</div>
  }

  return (
    <div>Porla</div>
  );
}

export default Home;
