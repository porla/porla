import { createFileRoute, Link, Outlet, useMatches } from '@tanstack/react-router'
import { useMemo } from "react";

export const Route = createFileRoute('/_auth/_indexLayout/_tabLayout')({
  component: () => {
    const matches = useMatches();
    const generalTabMatch = useMemo(() => matches.some(elem => elem.id === '/_auth/_indexLayout/_tabLayout/'), [matches]);
    const trackerTabMatch = useMemo(() => matches.some(elem => elem.id === '/_auth/_indexLayout/_tabLayout/tracker'), [matches]);

    return (
      <div className="flex flex-col p-1 w-full">
        <div className="flex">
          <ul className="flex flex-wrap">
            <li
              className={`${generalTabMatch ? 'border-t-4 border-x-4 border-neutral-800 rounded-t-lg' : 'mt-1 px-1 border-b-4 border-neutral-800'}`}>
              <Link to="/" className={`inline-block p-2 text-sm text-center ${generalTabMatch ? '' : ''}`}>
                General
              </Link>
            </li>
            <li
              className={`${trackerTabMatch ? 'border-t-4 border-x-4 border-neutral-800 rounded-t-lg' : 'mt-1 px-1 border-b-4 border-neutral-800'}`}>
              <Link to="/tracker" className={`inline-block p-2 text-sm text-center ${trackerTabMatch ? '' : ''}`}>
                Tracker
              </Link>
            </li>
          </ul>
          <div className="flex-grow border-b-4 border-neutral-800 "></div>
        </div>
        <div className="flex h-full w-full p-2">
          <Outlet/>
        </div>
      </div>
    );
  }
});
