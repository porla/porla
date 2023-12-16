import { ProgressBar } from "../ProgressBar.tsx";
import { applyRatioColor } from "../functions/applyRatioColor.tsx";
import { torrents } from "../MockData.tsx";

export const TorrentList = () => {
  return (
    <>
      <main className="h-full px-4 sm:px-6 lg:px-8 py-0 sm:py-10 overflow-auto">
        <table className="min-w-full divide-y divide-gray-700">
          <thead>
          <tr className="text-sm font-semibold text-neutral-300">
            <th scope="col" className="px-3 sm:px-0 py-3.5 text-left">Name</th>
            <th scope="col" className="px-3 py-3.5 text-right">Size</th>
            <th scope="col" className="px-3 py-3.5 text-center">Progress</th>
            <th scope="col" className="px-3 py-3.5 text-left">Status</th>
            <th scope="col" className="px-3 py-3.5 text-left">Download Speed</th>
            <th scope="col" className="px-3 py-3.5 text-left">Upload Speed</th>
            <th scope="col" className="px-3 py-3.5 text-left">Seeds</th>
            <th scope="col" className="px-3 py-3.5 text-left">Peers</th>
            <th scope="col" className="px-3 py-3.5 text-left">Ratio</th>
          </tr>
          </thead>
          <tbody className="divide-y divide-gray-800">
          {torrents.map((torrent) => (
            <tr key={torrent.id} className="whitespace-nowrap text-sm text-neutral-300">
              <td className="px-3 py-4 sm:pl-0">{torrent.name}</td>
              <td className="px-3 py-4 text-right">{torrent.size}</td>
              <td className="px-3 py-4">
                <ProgressBar progress={torrent.progress} gradient={true} fromColor="from-purple-500"
                             toColor="to-blue-600"/>
              </td>
              <td className="px-3 py-4 text-left">{torrent.status}</td>
              <td className="px-3 py-4 text-left">{torrent.download_speed}</td>
              <td className="px-3 py-4 text-left">{torrent.upload_speed}</td>
              <td className="px-3 py-4 text-left">{torrent.active_seeds} / {torrent.inactive_seeds}</td>
              <td className="px-3 py-4 text-left">{torrent.active_peers} / {torrent.inactive_peers}</td>
              <td className="px-3 py-4 text-left ">
                <span className={`${applyRatioColor(torrent.ratio)}`}>{torrent.ratio.toFixed(2)}</span>
              </td>
            </tr>
          ))}
          </tbody>
        </table>
      </main>
    </>
  );
};