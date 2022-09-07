import React from "react";
import { Center, Spinner, VStack, Text } from "@chakra-ui/react";
import { useTranslation } from "react-i18next";

export default function Loading() {
  const { t } = useTranslation();

  return (
    <Center h="100%">
      <VStack spacing={4}>
        <Spinner
          color="blue.500"
          emptyColor="gray.200"
          size="xl"
          speed="0.65s"
          thickness="5px"
        />
        <Text>
          {t('loading')}
        </Text>
      </VStack>
    </Center>
  );
}
