import {
  CloudArrowDownIcon,
  CloudArrowUpIcon,
  PlayIcon,
  PauseIcon,
  Cog6ToothIcon,
  PuzzlePieceIcon,
  MagnifyingGlassIcon
} from "@heroicons/react/24/solid";
import PorlaIcon from '../../porla.svg?react'

export const Toolbar = () => {
  return (
    <>
      <div className="flex justify-between items-center dark:bg-neutral-800 border-b border-b-neutral-600">
        <div className="grid grid-cols-3 w-full h-10">
          <div className="flex pl-4 gap-x-6 items-center">
            <div className="flex items-center">
              <PorlaIcon className="h-6 w-6"/>
              <span className="pl-2 text-2xl font-bold">Porla</span>
            </div>
            <div className="flex gap-x-2">
              <a href="#"><CloudArrowUpIcon className="h-8"/></a>
              <a href="#"><CloudArrowDownIcon className="h-8"/></a>
              <a href="#"><PlayIcon className="h-8 fill-green-600"/></a>
              <a href="#"><PauseIcon className="h-8 -mx-1 fill-amber-600"/></a>
            </div>
          </div>
          <div className="flex justify-center py-1">
            <label className="relative flex">
            <span className="absolute inset-y-0 left-0 flex items-center pl-2">
                <MagnifyingGlassIcon className="h-6 stroke-current"/>
            </span>
              <input className="pl-10 rounded-full w-96" placeholder="Filter torrents..."/>
            </label>
          </div>
          <div className="flex pr-4 items-center justify-end">
            <div className="flex gap-x-2">
              <a href="#"><PuzzlePieceIcon className="h-8"/></a>
              <a href="#"><Cog6ToothIcon className="h-8"/></a>
            </div>
          </div>
        </div>

      </div>
    </>
  )
}
