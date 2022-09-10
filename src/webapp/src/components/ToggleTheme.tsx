import React from "react";
import { Button, useColorMode } from "@chakra-ui/react";
import { MdDarkMode, MdLightMode } from "react-icons/md";
import { useTranslation } from "react-i18next";

function ToggleTheme() {
  const { t } = useTranslation();
  const { colorMode, toggleColorMode } = useColorMode();

  return (
    <Button
      aria-label={t("change_theme")}
      leftIcon={colorMode === "light" ? <MdDarkMode/> : <MdLightMode/>}
      onClick={toggleColorMode}
    >
      {t(`toggle_theme_${colorMode}`)}
    </Button>
  )
}

export default ToggleTheme;