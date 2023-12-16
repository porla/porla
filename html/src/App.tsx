import { useEffect, useState } from 'react'
import { Allotment } from "allotment";
import { TorrentList } from "./components/panes/TorrentList.tsx";
import { NavigationSidebar } from "./components/panes/NavigationSidebar.tsx";
import { MobileNavigation } from "./components/panes/MobileNavigation.tsx";

function App() {

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
      <Allotment className="h-screen hidden lg:flex"
                 defaultSizes={sidebarSize} onDragEnd={setSidebarSize}>
        <Allotment.Pane maxSize={300}>
          <NavigationSidebar/>
        </Allotment.Pane>
        <Allotment className="h-screen" defaultSizes={mainAreaSize} onDragEnd={setMainAreaSize} vertical={true}>
          <TorrentList/>
          <div className="flex h-full justify-center items-center">
            <span>This will be the details area but it has not been started yet.</span>
          </div>
        </Allotment>
      </Allotment>
      <div className="flex flex-col h-screen lg:hidden">
        <MobileNavigation/>
        <TorrentList/>
      </div>
    </>
  )
}

export default App
