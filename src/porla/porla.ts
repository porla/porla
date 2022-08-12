import { Database as DatabaseType } from "better-sqlite3";
import express from "express";
import { ISession } from "./session";

export type PluginLoader = (host: IHost) => Promise<void>;

export interface IHost {
  database(): DatabaseType;
  express(): express.Application;
  session(): ISession;
}
