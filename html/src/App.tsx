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


const navigation = [


  { name: 'All', href: '#', icon: CloudIcon, current: true, count: 122 },
  { name: 'Downloading', href: '#', icon: CloudArrowDownIcon, current: false, count: 33 },
  { name: 'Seeding', href: '#', icon: CloudArrowUpIcon, current: false, count: 89 },
  { name: 'Error', href: '#', icon: ExclamationTriangleIcon, current: false, count: 0 },
]
const teams = [


]

const torrents = [


  { name: 'debian-12.2.0-amd64-netinst.iso', size: '287.4 MiB', email: 'lindsay.walton@example.com', role: 'Member' },
  // More people...
]

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
          <div className="flex grow flex-col gap-y-5 overflow-y-auto border-r border-gray-200 dark:border-neutral-600 bg-white dark:bg-neutral-800 px-6">
            <div className="flex h-16 shrink-0 items-center">
              <img
                className="h-8 w-auto"
                src="/isotype.svg"
                alt="Porla"
              />
            </div>
            <nav className="flex flex-1 flex-col">
              <div className="text-xs font-semibold leading-6 text-gray-400 dark:text-neutral-400">Torrents</div>
              <ul role="list" className="flex flex-1 flex-col gap-y-7">
                <li>
                  <ul role="list" className="-mx-2 space-y-1">
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
                </li>
                <li>
                  <div className="text-xs font-semibold leading-6 text-gray-400">Trackers</div>
                  <ul role="list" className="-mx-2 mt-2 space-y-1">
                    {teams.map((team) => (
                      <li key={team.name}>
                        <a
                          href={team.href}
                          className={classNames(
                            team.current
                              ? 'bg-gray-50 text-indigo-600'
                              : 'text-gray-700 hover:text-indigo-600 hover:bg-gray-50',
                            'group flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold'
                          )}
                        >
                          <span
                            className={classNames(
                              team.current
                                ? 'text-indigo-600 border-indigo-600'
                                : 'text-gray-400 border-gray-200 group-hover:border-indigo-600 group-hover:text-indigo-600',
                              'flex h-6 w-6 shrink-0 items-center justify-center rounded-lg border text-[0.625rem] font-medium bg-white'
                            )}
                          >
                            {team.initial}
                          </span>
                          <span className="truncate">{team.name}</span>
                        </a>
                      </li>
                    ))}
                  </ul>
                </li>
                <li className="-mx-6 mt-auto">
                  <a
                    href="#"
                    className="flex items-center gap-x-4 px-6 py-3 text-sm font-semibold leading-6 text-gray-900 hover:bg-gray-50"
                  >
                    <img
                      className="h-8 w-8 rounded-full bg-gray-50"
                      src="https://images.unsplash.com/photo-1472099645785-5658abf4ff4e?ixlib=rb-1.2.1&ixid=eyJhcHBfaWQiOjEyMDd9&auto=format&fit=facearea&facepad=2&w=256&h=256&q=80"
                      alt=""
                    />
                    <span className="sr-only">Your profile</span>
                    <span aria-hidden="true">Tom Cook</span>
                  </a>
                </li>
              </ul>
            </nav>
          </div>
        </div>

        <div className="sticky top-0 z-40 flex items-center gap-x-6 bg-white px-4 py-4 shadow-sm sm:px-6 lg:hidden">
          <button type="button" className="-m-2.5 p-2.5 text-gray-700 lg:hidden" onClick={() => setSidebarOpen(true)}>
            <span className="sr-only">Open sidebar</span>
            <Bars3Icon className="h-6 w-6" aria-hidden="true" />
          </button>
          <div className="flex-1 text-sm font-semibold leading-6 text-gray-900">Dashboard</div>
          <a href="#">
            <span className="sr-only">Your profile</span>
            <img
              className="h-8 w-8 rounded-full bg-gray-50"
              src="https://images.unsplash.com/photo-1472099645785-5658abf4ff4e?ixlib=rb-1.2.1&ixid=eyJhcHBfaWQiOjEyMDd9&auto=format&fit=facearea&facepad=2&w=256&h=256&q=80"
              alt=""
            />
          </a>
        </div>

        <main className="py-10 lg:pl-72">
          <div className="px-4 sm:px-6 lg:px-8">
          <table className="min-w-full divide-y divide-gray-700">
                    <thead>
                      <tr>
                        <th scope="col" className="py-3.5 pl-4 pr-3 text-left text-sm font-semibold text-neutral-300 sm:pl-0">
                          Name
                        </th>
                        <th scope="col" className="px-3 py-3.5 text-right text-sm font-semibold text-neutral-300">
                          Size
                        </th>
                        <th scope="col" className="px-3 py-3.5 text-center text-sm font-semibold text-neutral-300">
                          Progress
                        </th>
                        <th scope="col" className="px-3 py-3.5 text-right text-sm font-semibold text-neutral-300">
                          Peers
                        </th>
                      </tr>
                    </thead>
                    <tbody className="divide-y divide-gray-800">
                      {torrents.map((torrent) => (
                        <tr key={torrent.email}>
                          <td className="whitespace-nowrap py-4 pl-4 pr-3 text-sm text-neutral-300 sm:pl-0">
                            {torrent.name}
                          </td>
                          <td className="whitespace-nowrap px-3 py-4 text-sm text-right text-gray-300">{torrent.size}</td>
                          <td className="whitespace-nowrap px-3 py-4 text-sm text-gray-300">
                            <progress className="progress w-100" value="67" max="100"></progress>
                          </td>
                          <td className="whitespace-nowrap px-3 py-4 text-sm text-right text-gray-300">10 / 100</td>
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
