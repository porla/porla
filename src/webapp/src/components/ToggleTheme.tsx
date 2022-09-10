import React from "react";
import { Button, useColorMode } from "@chakra-ui/react";
import { BsSun, BsMoon } from "react-icons/bs";
import { useTranslation } from "react-i18next";

function ToggleTheme() {
  const { t } = useTranslation();
  const { colorMode, toggleColorMode } = useColorMode();

  return (
    <Button
      aria-label={t("change_theme")}
      leftIcon={colorMode === "light" ? <BsMoon/> : <BsSun/>}
      onClick={toggleColorMode}
    >
      {t(`toggle_theme_${colorMode}`)}
    </Button>
  )
}

export default ToggleTheme;