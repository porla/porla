import React from "react";
import { IconButton, useColorMode } from "@chakra-ui/react";
import { MdLightMode, MdDarkMode } from "react-icons/md";

function ToggleTheme() {
  const { colorMode, toggleColorMode } = useColorMode()
  return (
    <IconButton
      aria-label=""
      icon={colorMode === 'light' ? <MdDarkMode/> : <MdLightMode/>}
      onClick={toggleColorMode}
    />
  )
}

export default ToggleTheme;