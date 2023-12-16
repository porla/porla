import { CollapsibleSection } from "../CollapsibleSection.tsx";
import { navigation, trackers } from "../MockData.tsx";
import PorlaIcon from "../../porla.svg?react"
import { classNames } from "../functions/classNames.tsx";

export const NavigationSidebar = () => {
  return (
    <>
      <nav
        className="h-full flex flex-1 flex-col overflow-y-auto border-r border-gray-200 dark:border-neutral-600 bg-white dark:bg-neutral-800 px-4">
        <ul role="list" className="flex flex-1 flex-col gap-y-7 pt-4">
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
              className="flex items-center gap-x-4 mb-2 px-2 py-2 text-sm font-semibold leading-6 text-neutral-300 hover:bg-gray-50 hover:dark:bg-neutral-700 rounded-md"
            >
              <PorlaIcon className="h-8 w-auto"/>
              <span className="sr-only">Your profile</span>
              <span aria-hidden="true">Porla User</span>
            </a>
          </li>
        </ul>
      </nav>
    </>
  )
}