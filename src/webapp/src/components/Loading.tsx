import { Center, Spinner, VStack, Text } from "@chakra-ui/react";
import React from "react";

export default function Loading() {
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
          Loading...
        </Text>
      </VStack>
    </Center>
  );
}
