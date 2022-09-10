
function getState(torrent: any): string {
  if (!torrent) return "unknown";

  const { state, flags, progress } = torrent;

  if (progress === 1 && isCompleted(torrent)) {
    return "completed";
  }

  if (progress === 1) {
    return "seeding";
  }

  if (isPaused(flags)) {
    return "paused";
  }

  if (state === 3 && !isCompleted(torrent)) {
    return "downloading";
  }

  return "unknown";
}

function getStateColor(state: string): string {
  const colors = {
    "completed": "blue.500",
    "downloading": "cyan.500",
    "error": "red.500",
    "paused": "gray.500",
    "seeding": "green.500",
    "unknown": "red.500",
  };

  return colors[state];
}


function isPaused(flags: number): boolean {
  return (flags & (1<<4)) === 1<<4;
}

function isCompleted(torrent: { state: number, flags: number }): boolean {
  return torrent.state === 5 && isPaused(torrent.flags);
}

export {
  getState,
  getStateColor,
  isPaused,
  isCompleted,
}