import { createFileRoute } from '@tanstack/react-router'

export const Route = createFileRoute('/_auth/_indexLayout/_tabLayout/')({
  component: () => <div>General tab content</div>
})