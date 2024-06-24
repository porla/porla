import { useEffect, useState } from 'react'
import { Allotment } from "allotment";
import { TorrentList } from "./components/panes/TorrentList.tsx";
import { NavigationSidebar } from "./components/panes/NavigationSidebar.tsx";
import { MobileNavigation } from "./components/panes/MobileNavigation.tsx";
import { Toolbar } from "./components/panes/Toolbar.tsx";
import { Outlet } from "@tanstack/react-router";

function Index() {

  const storedSidebarSize = JSON.parse(localStorage.getItem('sidebarSize') as string) || []
  const [ sidebarSize, setSidebarSize ] = useState(storedSidebarSize)

  useEffect(() => {
    localStorage.setItem(`sidebarSize`, JSON.stringify(sidebarSize))
  }, [ sidebarSize ])

  useEffect(() => {
    const sidebarSize = JSON.parse(localStorage.getItem('sidebarSize') as string);
    if (sidebarSize) {
      setSidebarSize(sidebarSize);
    }
  }, [ setSidebarSize ]);

  const storedMainAreaSize = JSON.parse(localStorage.getItem('mainAreaSize') as string) || []
  const [ mainAreaSize, setMainAreaSize ] = useState(storedMainAreaSize)

  useEffect(() => {
    localStorage.setItem(`mainAreaSize`, JSON.stringify(mainAreaSize))
  }, [ mainAreaSize ])

  useEffect(() => {
    const MainAreaSize = JSON.parse(localStorage.getItem('mainAreaSize') as string);
    if (MainAreaSize) {
      setMainAreaSize(MainAreaSize);
    }
  }, [ setMainAreaSize ]);

  return (
    <>
      <div className="h-screen hidden lg:flex lg:flex-col">
        <Toolbar/>
        <Allotment className="h-full hidden lg:flex"
                   defaultSizes={sidebarSize} onDragEnd={setSidebarSize}>
          <Allotment.Pane maxSize={300}>
            <NavigationSidebar/>
          </Allotment.Pane>
          <Allotment className="h-full" defaultSizes={mainAreaSize} onDragEnd={setMainAreaSize} vertical={true}>
            <TorrentList/>
            <div className="flex h-full min-w-full">
              <Outlet />
            </div>
          </Allotment>
        </Allotment>
      </div>
      <div className="flex flex-col h-screen lg:hidden">
        <MobileNavigation/>
        <TorrentList/>
      </div>
    </>
  )
}

export default Index
