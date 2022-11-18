import { Button, Text } from "@chakra-ui/react";

type IPagerProps = {
  page: number;
  pageSize: number;
  totalItems: number;
  setPage: (p: number) => void;
}

export default function Pager(props: IPagerProps) {
  const totalPages = Math.ceil(props.totalItems / props.pageSize);
  const pageElements = [];
  const surround = 5;

  const begin = Math.max(0, props.page - surround);
  const end = Math.min(totalPages, props.page + surround);

  for (let i = begin; i < end; i++) {
    pageElements.push(
      <Button
        key={i}
        ml={1}
        size={"xs"}
        variant={props.page === i ? "solid" : "link"}
        onClick={() => props.setPage(i)}
      >
        {i+1}
      </Button>
    );
  }

  if (totalPages > end) {
    pageElements.push(
      <Button
        size={"xs"}
        variant={"link"}
      >
        &hellip;
      </Button>
    );

    pageElements.push(
      <Button
        ml={1}
        size={"xs"}
        variant={"link"}
      >
        {totalPages}
      </Button>
    )
  }

  return (
    <>
      { pageElements }
    </>
  )
}
