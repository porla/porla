import { createFileRoute } from '@tanstack/react-router'

export const Route = createFileRoute('/_auth/_details')({
  component: () => <div>Hello /_auth/_details!</div>
})