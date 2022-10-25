import { Button } from "@chakra-ui/react";

type IPagerProps = {
  page: number;
  pageSize: number;
  totalItems: number;
  setPage: (p: number) => void;
}

export default function Pager(props: IPagerProps) {
  const totalPages = Math.ceil(props.totalItems / props.pageSize);

  const pageButtons = [];

  for (let i = 0; i < totalPages; i++) {
    pageButtons.push(
      <Button
        ml={1}
        size={"xs"}
        variant={props.page === i ? "solid" : "link"}
        onClick={() => props.setPage(i)}
      >
        {i+1}
      </Button>
    );
  }

  return (
    <>
        {props.totalItems}
      { pageButtons }
    </>
  )
}
