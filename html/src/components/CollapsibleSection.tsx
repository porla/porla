import { ReactNode, useState } from "react";
import {
  ChevronRightIcon,
  ChevronDownIcon
} from "@heroicons/react/24/outline";

interface CollapsibleSectionProps {
  title: string;
  children: ReactNode;
}

export function CollapsibleSection({ title, children }: CollapsibleSectionProps) {
  const [isOpen, setIsOpen] = useState(true);

  const toggleOpen = () => setIsOpen(!isOpen);

  return (
    <div className="">
      <button onClick={toggleOpen} className="flex justify-between items-center">
        {isOpen ? <ChevronDownIcon className="h-4 w-4"/> : <ChevronRightIcon className="h-4 w-4"/>}
        <span className="pl-1 text-sm font-semibold leading-6 text-gray-400 dark:text-neutral-400">{title}</span>
      </button>
      {isOpen && <div>{children}</div>}
    </div>
  );
}