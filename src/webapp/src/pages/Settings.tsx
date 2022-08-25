import { Box, Heading } from "@chakra-ui/react";
import React from "react";

export default function Settings() {
  return (
    <Box
      backgroundColor={"#fff"}
      border="1px solid #f0f0f0"
      borderRadius={4}
      padding={3}
    >
      <Heading
        color={"#444"}
        marginBottom={3}
        size="md"
      >
        General
      </Heading>
    </Box>
  );
}
