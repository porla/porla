import { Bars3Icon } from "@heroicons/react/24/outline";

export const MobileNavigation = () => {
  return (
    <>
      <div className="flex sticky top-0 items-center bg-white dark:bg-neutral-800 px-4 py-4 shadow-sm sm:px-6">
        <button type="button"
                className="pr-3 text-gray-700" /* onClick={() => setSidebarOpen(true)} */>
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
    </>
  )
}