import "../scss/components/sidebar.scss";

import React from "react";
import { NavLink } from "react-router-dom";
import { Box, Button, HStack, VStack, Flex, Icon, IconButton, Link, useColorMode } from "@chakra-ui/react";
import { BsColumnsGap, BsGear, BsCollection, BsPlusCircle } from "react-icons/bs";
import { useTranslation } from "react-i18next";
import { Porla } from "./brand";

export default function Sidebar() {
  const { t } = useTranslation();
  const { colorMode } = useColorMode();

  const menuSettings = {
    size: "md",
    paddingY: 2,
    paddingX: 3,
    iconSize: 5,
    iconMargin: 2,
    active: {
      fontWeight: 700,
      background: colorMode === "light" ? "blackAlpha.200" : "whiteAlpha.100",
    }
  }

  const menu = [
    {
      slug: "home",
      to: "/",
      leftIcon: {
        as: BsColumnsGap
      },
    }, {
      slug: "torrents",
      to: "/torrents",
      leftIcon: {
        aria: "torrents",
        as: BsCollection,
      },
      rightIcon: {
        aria: "add_torrent",
        as: NavLink,
        icon: <BsPlusCircle />,
        to: "/torrents/add",
      },
    },
  ];

  const menuBottom = {
    slug: "settings",
    to: "/settings",
    leftIcon: {
      aria: "settings",
      as: BsGear,
    },
  };

  const SidebarItem = (item: any) => {
    return (
      <HStack width="100%" alignItems="stretch" key={item.slug} position="relative">
        <Button
          _activeLink={menuSettings.active}
          flex={1}
          variant="ghost"
          justifyContent="start"
          as={NavLink}
          to={item.to}
          py={menuSettings.paddingY}
          px={menuSettings.paddingX}
          size={menuSettings.size}
          leftIcon={item.leftIcon && (
            <Icon
              className="btn-left"
              as={item.leftIcon.as}
              me={menuSettings.iconMargin}
              w={menuSettings.iconSize}
              h={menuSettings.iconSize}
            />
          )}
        >
          <Box flex="auto">
            {t(item.slug)}
          </Box>
        </Button>
        {item.rightIcon &&
          <IconButton
            className="btn-right"
            position="absolute"
            right="0"
            aria-label={t(item.rightIcon.slug)}
            as={Link}
            icon={item.rightIcon.icon}
            href={item.rightIcon.to}
            borderRadius="full"
            variant="ghost"
            size="lg"
          />
        }
      </HStack>
    );
  }

  return (
    <Flex
      id="sidebar"
      className="sidebar"
      borderRightWidth={1}
      direction="column"
      justifyContent="flex-end"
      height="100%"
      width="300px"
      px={3}
      py={5}
    >
      <Box flex={1}>
        <Box mb={10} px={2}>
          <Link href="./">
            <Porla height="32px"/>
          </Link>
        </Box>

        <VStack>
          {menu.map(item => SidebarItem(item))}
        </VStack>
      </Box>

      <Box>
        {SidebarItem(menuBottom)}
      </Box>
    </Flex>
  );
}