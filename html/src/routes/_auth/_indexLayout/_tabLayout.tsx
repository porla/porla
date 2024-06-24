import { createFileRoute } from '@tanstack/react-router'
import { Details } from "../../../components/panes/Details.tsx";

export const Route = createFileRoute('/_auth/_indexLayout/_tabLayout')({
  component: Details
});
