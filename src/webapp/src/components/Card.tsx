import React from "react";
import { Box, BoxProps, Heading } from "@chakra-ui/react";

interface CardProps extends BoxProps {
  children: JSX.Element[] | JSX.Element | string;
  heading?: string;
}

export default function Card(props: CardProps) {
  return (
    <Box
      borderWidth={1}
      borderRadius={4}
      padding={5}
      { ...props }
    >
      <Heading
        marginBottom={3}
        size="md"
      >
        { props.heading }
      </Heading>

      { props.children }
    </Box>
  );
}
