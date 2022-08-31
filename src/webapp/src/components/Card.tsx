import { Box, BoxProps, Heading } from "@chakra-ui/react";
import React from "react";

interface CardProps extends BoxProps {
  children: JSX.Element[] | JSX.Element | string;
  heading?: string;
}

export default function Card(props: CardProps) {
  return (
    <Box
      backgroundColor={"#fff"}
      border="1px solid #f0f0f0"
      borderRadius={4}
      padding={3}
      { ...props }
    >
      <Heading
        color={"#444"}
        marginBottom={3}
        size="md"
      >
        { props.heading }
      </Heading>

      { props.children }
    </Box>
  );
}
