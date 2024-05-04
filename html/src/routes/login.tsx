import * as React from 'react'
import {
  createFileRoute,
  redirect,
  useRouter,
  useRouterState,
} from '@tanstack/react-router'
import { z } from 'zod'

import { useAuth } from '../auth'
import PorlaIcon from '../porla.svg?react'

const fallback = '/' as const

export const Route = createFileRoute('/login')({
  validateSearch: z.object({
    redirect: z.string().optional().catch(''),
  }),
  beforeLoad: ({ context, search }) => {
    if (context.auth.isAuthenticated) {
      throw redirect({ to: search.redirect || fallback })
    }
  },
  component: LoginComponent,
})

function LoginComponent() {
  const auth = useAuth()
  const router = useRouter()
  const isLoading = useRouterState({ select: (s) => s.isLoading })
  const navigate = Route.useNavigate()

  const search = Route.useSearch()

  const onFormSubmit = (e: React.FormEvent<HTMLFormElement>) => {
    e.preventDefault()
    const data = new FormData(e.currentTarget)
    const fieldValue = data.get('username')

    if (!fieldValue) return

    const username = fieldValue.toString()

    auth.login(username)

    router.invalidate().finally(() => {
      navigate({ to: search.redirect || fallback })
    })
  }

  return (
    <div className="min-h-screen flex flex-col justify-center items-center bg-neutral-50 dark:bg-neutral-900">
      <div className="flex justify-center max-w-md w-full p-0.5 rounded-3xl bg-gradient-to-b from-blue-500 to-purple-500">
        <form className="flex flex-col max-w-md w-full p-10 rounded-3xl bg-neutral-50 dark:bg-neutral-800 gap-y-6" onSubmit={onFormSubmit}>
          <div className="flex flex-row items-center justify-center">
            <PorlaIcon className="size-12"/>
            <span className="pl-4 text-5xl font-bold">Porla</span>
          </div>
          <label className="input flex items-center gap-2 bg-neutral-50 dark:bg-neutral-800 border border-neutral-700">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" fill="currentColor"
                 className="w-4 h-4 opacity-70">
              <path
                d="M8 8a3 3 0 1 0 0-6 3 3 0 0 0 0 6ZM12.735 14c.618 0 1.093-.561.872-1.139a6.002 6.002 0 0 0-11.215 0c-.22.578.254 1.139.872 1.139h9.47Z"/>
            </svg>
            <input name="username" type="text" className="grow" placeholder="Username" required={true} />
          </label>
          <label className="input flex items-center gap-2 bg-neutral-50 dark:bg-neutral-800 border border-neutral-700">
            <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 16 16" fill="currentColor"
                 className="w-4 h-4 opacity-70">
              <path fillRule="evenodd"
                    d="M14 6a4 4 0 0 1-4.899 3.899l-1.955 1.955a.5.5 0 0 1-.353.146H5v1.5a.5.5 0 0 1-.5.5h-2a.5.5 0 0 1-.5-.5v-2.293a.5.5 0 0 1 .146-.353l3.955-3.955A4 4 0 1 1 14 6Zm-4-2a.75.75 0 0 0 0 1.5.5.5 0 0 1 .5.5.75.75 0 0 0 1.5 0 2 2 0 0 0-2-2Z"
                    clipRule="evenodd"/>
            </svg>
            <input type="password" className="grow" placeholder="Password"/>
          </label>
          <button type="submit" disabled={isLoading} className="btn w-full text-white bg-neutral-50 dark:bg-blue-500 border border-neutral-700">
            {isLoading ? 'Loading spinner should go brr here...' : 'Login'}
          </button>
          </form>
        </div>
      {/* Could do this here....
      {search.redirect ? (
        <p className="text-red-500">You need to login to access this page.</p>
      ) : (
        <p>Login to see all the cool content in here.</p>
      )}*/}
    </div>
  )
}
