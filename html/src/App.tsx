import { Fragment, useState } from 'react'
import { Dialog, Transition } from '@headlessui/react'
import {
  Bars3Icon,
  ChevronLeftIcon,
  ChevronRightIcon,
  CloudArrowDownIcon,
  CloudArrowUpIcon,
  CloudIcon,
  ExclamationTriangleIcon,
  XMarkIcon,
} from '@heroicons/react/24/outline'

import PorlaLogo from "./porla.svg?react";

import { ProgressBar } from "./components/ProgressBar.tsx";
import { CollapsibleSection } from "./components/CollapsibleSection.tsx";


const navigation = [

  { name: 'All', href: '#', icon: CloudIcon, current: true, count: 122 },
  { name: 'Downloading', href: '#', icon: CloudArrowDownIcon, current: false, count: 33 },
  { name: 'Seeding', href: '#', icon: CloudArrowUpIcon, current: false, count: 89 },
  { name: 'Error', href: '#', icon: ExclamationTriangleIcon, current: false, count: 0 },

]

const trackers = [

  { url: "https://tracker1.porla.org/announce/s0m3p455k3y", href: '#', icon: PorlaLogo, current: true, count: 2 },
  { url: "https://tracker2.porla.org/announce/4n0th3rp455k3y", href: '#', icon: PorlaLogo, current: false, count: 0 }

]

const torrents = [

  {
    id: 1,
    name: 'debian-12.2.0-amd64-netinst.iso',
    size: '287.4 MiB',
    progress: 99,
    status: 'Downloading',
    download_speed: '117.00 MiB/s',
    upload_speed: '117.00 MiB/s',
    active_seeds: 1,
    inactive_seeds: 2,
    active_peers: 3,
    inactive_peers: 4,
    ratio: 13.37
  },
  {
    id: 2,
    name: "archlinux-2023.12.01-x86_64.iso",
    size: "872 MiB",
    progress: 100,
    status: 'Seeding',
    download_speed: '1.17 GiB/s',
    upload_speed: '1.17 GiB/s',
    active_seeds: 5,
    inactive_seeds: 6,
    active_peers: 7,
    inactive_peers: 8,
    ratio: 133.7
  },
  {
    id: 3,
    name: "ubuntu-20.04.6-live-server-amd64.iso",
    size: "1.4 GiB",
    progress: 50,
    status: 'Downloading',
    download_speed: '11.70 MiB/s',
    upload_speed: '11.70 MiB/s',
    active_seeds: 9,
    inactive_seeds: 10,
    active_peers: 11,
    inactive_peers: 12,
    ratio: 1.337
  }

]

const getColor = (ratio: number) => {
  const colorMap = {
    'text-amber-200 drop-shadow-[0_0_5px_rgba(255,255,0,1)]': 100,
    'text-orange-500': 50,
    'text-purple-500': 10,
    'text-blue-500': 5,
    'text-green-500': 2,
    'text-neutral-300': 1
  };

  const colors = Object.keys(colorMap);
  const ratios = Object.values(colorMap);

  for (let i = 0; i < ratios.length; i++) {
    if (ratio >= ratios[i]) {
      return colors[i];
    }
  }

  return 'text-gray-500';

};

function classNames(...classes: any[]) {
  return classes.filter(Boolean).join(' ')
}

function App() {
  return (
    <>
      <div>
        {/* Static sidebar for desktop */}
        <div className="hidden lg:fixed lg:inset-y-0 lg:z-50 lg:flex lg:w-72 lg:flex-col">
          {/* Sidebar component, swap this element with another sidebar if you like */}
          <div
            className="flex grow flex-col gap-y-5 overflow-y-auto border-r border-gray-200 dark:border-neutral-600 bg-white dark:bg-neutral-800 px-4">
            <div className="flex h-16 shrink-0 items-center">
              <img
                className="h-8 w-auto"
                src="/isotype.svg"
                alt="Porla"
              />
            </div>
            <nav className="flex flex-1 flex-col">
              <ul role="list" className="flex flex-1 flex-col gap-y-7">
                <li>
                  <CollapsibleSection title="Torrents">
                    <ul role="list" className="mt-2 space-y-1">
                      {navigation.map((item) => (
                        <li key={item.name}>
                          <a
                            href={item.href}
                            className={classNames(
                              item.current
                                ? 'bg-gray-50 dark:bg-neutral-600 text-neutral-600 dark:text-neutral-200'
                                : 'text-neutral-700 dark:text-neutral-500 hover:text-neutral-600 hover:dark:text-neutral-200 hover:bg-neutral-50 hover:dark:bg-neutral-700',
                              'group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold'
                            )}
                          >
                            <item.icon
                              className={classNames(
                                item.current
                                  ? 'text-neutral-600 dark:text-neutral-200'
                                  : 'text-gray-400 dark:text-neutral-400 group-hover:text-neutral-600 group-hover:dark:text-neutral-200',
                                'h-6 w-6 shrink-0'
                              )}
                              aria-hidden="true"
                            />
                            {item.name}
                            {item.count >= 0 ? (
                              <span
                                className={classNames(
                                  item.current
                                    ? 'dark:ring-neutral-400'
                                    : 'group-hover:dark:ring-neutral-500 dark:ring-neutral-600',
                                  'ml-auto w-9 min-w-max whitespace-nowrap rounded-full bg-white dark:bg-neutral-800 px-2.5 py-0.5 text-center text-xs font-medium leading-5 text-gray-600 dark:text-neutral-400 ring-1 ring-inset ring-gray-200'
                                )}
                                aria-hidden="true"
                              >
                              {item.count}
                            </span>
                            ) : null}
                          </a>
                        </li>
                      ))}
                    </ul>
                  </CollapsibleSection>
                </li>
                <li>
                  <CollapsibleSection title="Trackers">
                    <ul role="list" className="mt-2 space-y-1">
                      {trackers.map((tracker) => (
                        <li key={tracker.url}>
                          <a
                            href={tracker.href}
                            className={classNames(
                              tracker.current
                                ? 'bg-gray-50 dark:bg-neutral-600 text-neutral-600 dark:text-neutral-200'
                                : 'text-neutral-700 dark:text-neutral-500 hover:text-neutral-600 hover:dark:text-neutral-200 hover:bg-neutral-50 hover:dark:bg-neutral-700',
                              'group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold'
                            )}
                          >
                            <tracker.icon
                              className={classNames(
                                tracker.current
                                  ? 'text-neutral-600 dark:text-neutral-200'
                                  : 'text-gray-400 dark:text-neutral-400 group-hover:text-neutral-600 group-hover:dark:text-neutral-200',
                                'h-6 w-6 shrink-0'
                              )}
                              aria-hidden="true"
                            />
                            <div className="truncate">
                              {(new URL(tracker.url)).hostname}
                            </div>
                            {tracker.count >= 0 ? (
                              <span
                                className={classNames(
                                  tracker.current
                                    ? 'dark:ring-neutral-400'
                                    : 'group-hover:dark:ring-neutral-500 dark:ring-neutral-600',
                                  'ml-auto w-9 min-w-max whitespace-nowrap rounded-full bg-white dark:bg-neutral-800 px-2.5 py-0.5 text-center text-xs font-medium leading-5 text-gray-600 dark:text-neutral-400 ring-1 ring-inset ring-gray-200'
                                )}
                                aria-hidden="true"
                              >
                              {tracker.count}
                            </span>
                            ) : null}
                          </a>
                        </li>
                      ))}
                    </ul>
                  </CollapsibleSection>
                </li>
                <li className="mt-auto">
                  <a
                    href="#"
                    className="flex items-center gap-x-4 my-2 px-2 py-1 text-sm font-semibold leading-6 text-neutral-300 hover:bg-gray-50 hover:dark:bg-neutral-700 rounded-md"
                  >
                    <img
                      className="h-8 w-8 rounded-full"
                      src="/isotype.svg"
                      alt=""
                    />
                    <span className="sr-only">Your profile</span>
                    <span aria-hidden="true">Porla User</span>
                  </a>
                </li>
              </ul>
            </nav>
          </div>
        </div>

        <div
          className="sticky top-0 z-40 flex items-center gap-x-6 bg-white dark:bg-neutral-800 px-4 py-4 shadow-sm sm:px-6 lg:hidden">
          <button type="button" className="-m-2.5 p-2.5 text-gray-700 lg:hidden" onClick={() => setSidebarOpen(true)}>
            <span className="sr-only">Open sidebar</span>
            <Bars3Icon className="h-6 w-6" aria-hidden="true"/>
          </button>
          <div className="flex-1 text-sm font-semibold leading-6 text-neutral-300">Dashboard</div>
          <a href="#">
            <span className="sr-only">Your profile</span>
            <img
              className="h-8 w-8 rounded-full"
              src="/isotype.svg"
              alt=""
            />
          </a>
        </div>

        <main className="py-0 sm:py-10 lg:pl-72">
          <div className="px-4 sm:px-6 lg:px-8 overflow-auto">
            <table className="min-w-full divide-y divide-gray-700">
              <thead>
              <tr className="text-sm font-semibold text-neutral-300">
                <th scope="col" className="px-3 sm:px-0 py-3.5 text-left">
                  Name
                </th>
                <th scope="col" className="px-3 py-3.5 text-right">
                  Size
                </th>
                <th scope="col" className="px-3 py-3.5 text-center">
                  Progress
                </th>
                <th scope="col" className="px-3 py-3.5 text-left">
                  Status
                </th>
                <th scope="col" className="px-3 py-3.5 text-left">
                  Download Speed
                </th>
                <th scope="col" className="px-3 py-3.5 text-left">
                  Upload Speed
                </th>
                <th scope="col" className="px-3 py-3.5 text-left">
                  Seeds
                </th>
                <th scope="col" className="px-3 py-3.5 text-left">
                  Peers
                </th>
                <th scope="col" className="px-3 py-3.5 text-left">
                  Ratio
                </th>
              </tr>
              </thead>
              <tbody className="divide-y divide-gray-800">
              {torrents.map((torrent) => (
                <tr key={torrent.id} className="whitespace-nowrap text-sm text-neutral-300">
                  <td className="px-3 py-4 sm:pl-0">
                    {torrent.name}
                  </td>
                  <td className="px-3 py-4 text-right">{torrent.size}</td>
                  <td className="px-3 py-4">
                    <ProgressBar progress={torrent.progress} gradient={true}/>
                  </td>
                  <td className="px-3 py-4 text-left">{torrent.status}</td>
                  <td className="px-3 py-4 text-left">{torrent.download_speed}</td>
                  <td className="px-3 py-4 text-left">{torrent.upload_speed}</td>
                  <td className="px-3 py-4 text-left">{torrent.active_seeds} / {torrent.inactive_seeds}</td>
                  <td className="px-3 py-4 text-left">{torrent.active_peers} / {torrent.inactive_peers}</td>
                  <td className="px-3 py-4 text-left ">
                    <span className={`${getColor(torrent.ratio)}`}>{torrent.ratio.toFixed(2)}</span>
                  </td>
                </tr>
              ))}
              </tbody>
            </table>
          </div>
        </main>
      </div>
    </>
  )
}

export default App
